
#include "dashboard.h"
#include "ui_dashboard.h"

struct Dashboard::Impl
{
};

Dashboard::Dashboard() : ui(new Ui::Dashboard), impl(std::make_unique<Impl>())
{
  ui->setupUi(this);
}

Dashboard::~Dashboard()
{
  delete ui;
}
