/*
  Copyright (c) 2015-2020 hkrn All rights reserved

  This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
*/

use super::native::*;

#[derive(Debug)]
pub struct Status {
    value: nanoem_status_t,
}
pub type Result<T> = std::result::Result<T, Status>;

impl Status {
    pub(crate) fn new(status: nanoem_status_t) -> Status {
        Status { value: status }
    }
    pub fn null_object() -> Status {
        Status::new(nanoem_status_t::ERROR_NULL_OBJECT)
    }
}

impl std::fmt::Display for Status {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{:?}", self.value)
    }
}

impl std::error::Error for Status {}

impl nanoem_status_t {
    pub fn closure<F, R>(self, callback: F) -> Result<R>
    where
        F: Fn() -> R,
    {
        match self {
            nanoem_status_t::SUCCESS => Ok(callback()),
            error => Err(Status::new(error)),
        }
    }
    pub fn value<T>(self, value: T) -> Result<T> {
        match self {
            nanoem_status_t::SUCCESS => Ok(value),
            error => Err(Status::new(error)),
        }
    }
    pub fn unit(self) -> Result<()> {
        self.value(())
    }
}

#[derive(Copy, Clone, Debug, Default)]
pub struct Interpolation {
    x0: u8,
    y0: u8,
    x1: u8,
    y1: u8,
}

impl Interpolation {
    pub fn new() -> Interpolation {
        Interpolation {
            x0: 20,
            y0: 20,
            x1: 107,
            y1: 107,
        }
    }
    #[allow(dead_code)]
    pub(crate) unsafe fn from_ptr(values: *const u8) -> Interpolation {
        Interpolation {
            x0: *values.add(0),
            y0: *values.add(1),
            x1: *values.add(2),
            y1: *values.add(3),
        }
    }
    pub fn as_ptr(self) -> *const u8 {
        &self.x0 as *const u8
    }
}

#[derive(Copy, Clone, Debug, Default)]
pub struct Vector4 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub w: f32,
}

impl Vector4 {
    pub fn new() -> Vector4 {
        Vector4 {
            x: 0.0,
            y: 0.0,
            z: 0.0,
            w: 1.0,
        }
    }
    pub(crate) unsafe fn from_ptr(values: *const f32) -> Vector4 {
        Vector4 {
            x: *values.add(0),
            y: *values.add(1),
            z: *values.add(2),
            w: *values.add(3),
        }
    }
    pub fn as_ptr(&self) -> *const f32 {
        &self.x as *const f32
    }
}

pub struct Buffer {
    pub(crate) opaque: *mut nanoem_buffer_t,
}

impl Buffer {
    pub fn from_slice(data: &[u8]) -> Result<Buffer> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemBufferCreate(data.as_ptr(), data.len(), status_ptr) };
        status.closure(|| Buffer { opaque })
    }
    pub fn copy_to(&self, data: &mut Vec<u8>) {
        unsafe {
            let size = nanoemBufferGetLength(self.opaque) as usize;
            data.clear();
            data.reserve(size);
            data.set_len(size);
            std::ptr::copy_nonoverlapping(
                nanoemBufferGetDataPtr(self.opaque),
                data.as_mut_ptr(),
                size,
            );
        }
    }
    pub fn from_mutable_buffer(buffer: &MutableBuffer) -> Result<Buffer> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMutableBufferCreateBufferObject(buffer.opaque, status_ptr) };
        status.closure(|| Buffer { opaque })
    }
    pub fn length(&self) -> usize {
        unsafe { nanoemBufferGetLength(self.opaque) as usize }
    }
    pub fn offset(&self) -> usize {
        unsafe { nanoemBufferGetOffset(self.opaque) as usize }
    }
}

impl std::fmt::Debug for Buffer {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"Buffer {{ length: {}, offset: {} }}"#,
            self.length(),
            self.offset(),
        )
    }
}

impl Drop for Buffer {
    fn drop(&mut self) {
        unsafe {
            nanoemBufferDestroy(self.opaque);
        }
    }
}

pub struct MutableBuffer {
    pub(crate) opaque: *mut nanoem_mutable_buffer_t,
}

impl MutableBuffer {
    pub fn new() -> Result<MutableBuffer> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMutableBufferCreate(status_ptr) };
        status.closure(|| MutableBuffer { opaque })
    }
}

impl Drop for MutableBuffer {
    fn drop(&mut self) {
        unsafe {
            nanoemMutableBufferDestroy(self.opaque);
        }
    }
}

pub struct UnicodeString<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    pub(crate) value: *mut nanoem_unicode_string_t,
}

impl<'a> Drop for UnicodeString<'a> {
    fn drop(&mut self) {
        unsafe {
            nanoemUnicodeStringFactoryDestroyString(self.unicode_string_factory.opaque, self.value)
        }
    }
}

pub struct UnicodeStringFactory {
    pub(crate) opaque: *mut nanoem_unicode_string_factory_t,
}

impl UnicodeStringFactory {
    pub fn create() -> Result<UnicodeStringFactory> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let factory = UnicodeStringFactory {
            opaque: unsafe { nanoemUnicodeStringFactoryCreateEXT(status_ptr) },
        };
        status.value(factory)
    }
    pub fn to_unicode_string(&self, value: &str) -> Result<UnicodeString> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            let us = nanoemUnicodeStringFactoryCreateString(
                self.opaque,
                value.as_ptr(),
                value.len(),
                status_ptr,
            );
            status.closure(|| UnicodeString {
                unicode_string_factory: self,
                value: us,
            })
        }
    }
    pub(crate) unsafe fn to_string(&self, value: *const nanoem_unicode_string_t) -> Result<String> {
        let mut length = 0usize;
        let length_ptr = &mut length as *mut usize;
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let bytes =
            nanoemUnicodeStringFactoryGetByteArray(self.opaque, value, length_ptr, status_ptr);
        let mut buffer = Vec::with_capacity(length);
        buffer.set_len(length);
        std::ptr::copy_nonoverlapping(bytes, buffer.as_mut_ptr(), length);
        nanoemUnicodeStringFactoryDestroyByteArray(self.opaque, bytes);
        status.value(String::from_utf8_unchecked(buffer))
    }
}

impl Drop for UnicodeStringFactory {
    fn drop(&mut self) {
        unsafe {
            nanoemUnicodeStringFactoryDestroyEXT(self.opaque);
        }
    }
}

#[derive(Copy, Clone, Debug)]
pub enum Language {
    Japanese,
    English,
}

impl Language {
    pub(crate) fn cast(self) -> nanoem_language_type_t {
        match self {
            Language::Japanese => nanoem_language_type_t::JAPANESE,
            Language::English => nanoem_language_type_t::ENGLISH,
        }
    }
}
