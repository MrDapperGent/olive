#include "exportvideotab.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

#include "core.h"
#include "render/colormanager.h"

ExportVideoTab::ExportVideoTab(ColorManager* color_manager, QWidget *parent) :
  QWidget(parent),
  color_manager_(color_manager)
{
  QVBoxLayout* outer_layout = new QVBoxLayout(this);

  outer_layout->addWidget(SetupResolutionSection());

  outer_layout->addWidget(SetupCodecSection());

  outer_layout->addWidget(SetupColorSection());

  outer_layout->addStretch();
}

QComboBox *ExportVideoTab::codec_combobox() const
{
  return codec_combobox_;
}

IntegerSlider *ExportVideoTab::width_slider() const
{
  return width_slider_;
}

IntegerSlider *ExportVideoTab::height_slider() const
{
  return height_slider_;
}

QCheckBox *ExportVideoTab::maintain_aspect_checkbox() const
{
  return maintain_aspect_checkbox_;
}

QComboBox *ExportVideoTab::scaling_method_combobox() const
{
  return scaling_method_combobox_;
}

const rational &ExportVideoTab::frame_rate() const
{
  return frame_rates_.at(frame_rate_combobox_->currentIndex());
}

void ExportVideoTab::set_frame_rate(const rational &frame_rate)
{
  frame_rate_combobox_->setCurrentIndex(frame_rates_.indexOf(frame_rate));
}

QString ExportVideoTab::CurrentOCIODisplay()
{
  return display_combobox_->currentData().toString();
}

QString ExportVideoTab::CurrentOCIOView()
{
  return views_combobox_->currentData().toString();
}

QString ExportVideoTab::CurrentOCIOLook()
{
  return looks_combobox_->currentData().toString();
}

CodecSection *ExportVideoTab::GetCodecSection() const
{
  return static_cast<CodecSection*>(codec_stack_->currentWidget());
}

void ExportVideoTab::SetCodecSection(CodecSection *section)
{
  codec_stack_->setCurrentWidget(section);
}

ImageSection *ExportVideoTab::image_section() const
{
  return image_section_;
}

H264Section *ExportVideoTab::h264_section() const
{
  return h264_section_;
}

QWidget* ExportVideoTab::SetupResolutionSection()
{
  int row = 0;

  QGroupBox* resolution_group = new QGroupBox();
  resolution_group->setTitle(tr("Basic"));

  QGridLayout* layout = new QGridLayout(resolution_group);

  layout->addWidget(new QLabel(tr("Width:")), row, 0);

  width_slider_ = new IntegerSlider();
  layout->addWidget(width_slider_, row, 1);

  row++;

  layout->addWidget(new QLabel(tr("Height:")), row, 0);

  height_slider_ = new IntegerSlider();
  layout->addWidget(height_slider_, row, 1);

  row++;

  layout->addWidget(new QLabel(tr("Maintain Aspect Ratio:")), row, 0);

  maintain_aspect_checkbox_ = new QCheckBox();
  maintain_aspect_checkbox_->setChecked(true);
  layout->addWidget(maintain_aspect_checkbox_, row, 1);

  row++;

  layout->addWidget(new QLabel(tr("Scaling Method:")), row, 0);

  scaling_method_combobox_ = new QComboBox();
  scaling_method_combobox_->setEnabled(false);
  scaling_method_combobox_->addItem(tr("Fit"), kFit);
  scaling_method_combobox_->addItem(tr("Stretch"), kStretch);
  scaling_method_combobox_->addItem(tr("Crop"), kCrop);
  layout->addWidget(scaling_method_combobox_, row, 1);

  // Automatically enable/disable the scaling method depending on maintain aspect ratio
  connect(maintain_aspect_checkbox_, &QCheckBox::toggled, this, &ExportVideoTab::MaintainAspectRatioChanged);

  row++;

  layout->addWidget(new QLabel(tr("Frame Rate:")), row, 0);

  frame_rate_combobox_ = new QComboBox();
  frame_rates_ = Core::SupportedFrameRates();
  foreach (const rational& fr, frame_rates_) {
    frame_rate_combobox_->addItem(Core::FrameRateToString(fr));
  }

  layout->addWidget(frame_rate_combobox_, row, 1);

  return resolution_group;
}

QWidget* ExportVideoTab::SetupColorSection()
{
  int row = 0;

  QGroupBox* color_group = new QGroupBox();
  color_group->setTitle(tr("Color Management"));

  QGridLayout* color_layout = new QGridLayout(color_group);

  color_layout->addWidget(new QLabel(tr("Display:")), row, 0);

  QStringList displays = color_manager_->ListAvailableDisplays();
  display_combobox_ = new QComboBox();
  foreach (const QString& display, displays) {
    display_combobox_->addItem(display, display);
  }
  connect(display_combobox_, SIGNAL(currentIndexChanged(int)), this, SLOT(ColorDisplayChanged()));
  color_layout->addWidget(display_combobox_, row, 1);

  row++;

  views_combobox_ = new QComboBox();
  color_layout->addWidget(new QLabel(tr("View:")), row, 0);
  color_layout->addWidget(views_combobox_, row, 1);
  connect(views_combobox_, SIGNAL(currentIndexChanged(int)), this, SLOT(ColorViewChanged()));

  row++;

  QStringList looks = color_manager_->ListAvailableLooks();
  looks_combobox_ = new QComboBox();
  looks_combobox_->addItem(tr("(None)"), QString());
  foreach (const QString& look, looks) {
    looks_combobox_->addItem(look, look);
  }
  connect(looks_combobox_, SIGNAL(currentIndexChanged(int)), this, SLOT(ColorLookChanged()));
  color_layout->addWidget(new QLabel(tr("Look:")), row, 0);
  color_layout->addWidget(looks_combobox_, row, 1);

  row++;

  ColorDisplayChanged();

  return color_group;
}

QWidget *ExportVideoTab::SetupCodecSection()
{
  int row = 0;

  QGroupBox* codec_group = new QGroupBox();
  codec_group->setTitle(tr("Codec"));

  QGridLayout* codec_layout = new QGridLayout(codec_group);

  codec_layout->addWidget(new QLabel(tr("Codec:")), row, 0);

  codec_combobox_ = new QComboBox();
  codec_layout->addWidget(codec_combobox_, row, 1);

  row++;

  codec_stack_ = new QStackedWidget();
  codec_layout->addWidget(codec_stack_, row, 0, 1, 2);

  image_section_ = new ImageSection();
  codec_stack_->addWidget(image_section_);

  h264_section_ = new H264Section();
  codec_stack_->addWidget(h264_section_);

  return codec_group;
}

void ExportVideoTab::ColorDisplayChanged()
{
  views_combobox_->clear();

  QStringList views = color_manager_->ListAvailableViews(display_combobox_->currentData().toString());
  foreach (const QString& view, views) {
    views_combobox_->addItem(view, view);
  }

  emit DisplayChanged(display_combobox_->currentData().toString());
}

void ExportVideoTab::ColorViewChanged()
{
  emit ViewChanged(views_combobox_->currentData().toString());
}

void ExportVideoTab::ColorLookChanged()
{
  emit LookChanged(looks_combobox_->currentData().toString());
}

void ExportVideoTab::MaintainAspectRatioChanged(bool val)
{
  scaling_method_combobox_->setEnabled(!val);
}
