/*
  Copyright (c) 2015-2020 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

pub(crate) const PLUGIN_MODEL_IO_ABI_VERSION: u32 = 2 << 16;
pub(crate) const PLUGIN_MOTION_IO_ABI_VERSION: u32 = 2 << 16;

#[allow(dead_code, non_camel_case_types)]
#[derive(Copy, Clone, Debug, Hash, PartialEq, PartialOrd)]
#[repr(i32)]
pub enum nanoem_application_plugin_status_t {
  ERROR_REFER_REASON = -3,
  ERROR_UNKNOWN_OPTION = -2,
  ERROR_NULL_OBJECT = -1,
  SUCCESS = 0,
}

impl nanoem_application_plugin_status_t {
  pub(crate) unsafe fn assign(self, value: *mut nanoem_application_plugin_status_t) {
    if !value.is_null() {
      *value = self;
    }
  }
}

mod model;
mod motion;
