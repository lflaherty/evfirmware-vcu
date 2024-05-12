/*
 * depends.h
 *
 *  Created on: 9 Nov 2022
 *      Author: Liam Flaherty
 */

#ifndef LIB_DEPENDS_DEPENDS_H_
#define LIB_DEPENDS_DEPENDS_H_

#include <stdint.h>
#include <stdbool.h>

#define MAX_NUM_MODULES 64

typedef uint32_t CompRegID_T;

/**
 * @brief Declare a struct to be a registerable module.
 * Insert anywhere in a module's struct.
 */
#define REGISTERED_MODULE() CompRegID_T compRegId

/**
 * @brief Declare a static module (a singleton module without a struct)
 * Insert in header.
 */
#define REGISTERED_MODULE_STATIC(name) extern CompRegID_T comp##name##RegId

/**
 * @brief Define a static module.
 * Insert into source file.
 */
#define REGISTERED_MODULE_STATIC_DEF(name) CompRegID_T comp##name##RegId = 0U

/**
 * @brief Register a module to be initialized and able to be depended on.
 * Returns an error from current method upon fail.
 * 
 * @param module Pointer to module struct.
 * @param ret_error Value to return on error.
 */
#define REGISTER(module, ret_error) \
  if (!RegisterModule(&(module)->compRegId)) { \
    return (ret_error); \
  }

/**
 * @brief Register a static module to be initialized and able to be depended on.
 * Returns an error from current method upon fail.
 * 
 * @param name Name of static module
 * @param ret_error Value to return on error.
 */
#define REGISTER_STATIC(name, ret_error) \
  if (!RegisterModule(&comp##name##RegId)) { \
    return (ret_error); \
  }

/**
 * @brief Depend on a module being registered.
 * Returns ret_error from current method if this is not true.
 * 
 * @param module Pointer to module struct.
 * @param ret_error Value to return on error.
 */
#define DEPEND_ON(module, ret_error) \
  if (NULL == module) { \
    return (ret_error); \
  } \
  if (!DependOn((module)->compRegId)) { \
    return (ret_error); \
  }

/**
 * @brief Depend on a static module being registered.
 * Returns ret_error from current method if this is not true.
 * 
 * @param name Name of static module
 * @param ret_error Value to return on error.
 */
#define DEPEND_ON_STATIC(name, ret_error) \
  if (!DependOn(comp##name##RegId)) { \
    return (ret_error); \
  }

/**
 * @brief Register a module to be initialized and able to be depended on.
 * 
 * @param newCompId Pointer to value to set to new component ID.
 * @return true if success.
 */
bool RegisterModule(CompRegID_T* newCompId);

/**
 * @brief Depend on a module being registered.
 * 
 * @param compId ID of component to depend on.
 * @return true if compId has been registered.
 */
bool DependOn(CompRegID_T compId);

#endif // LIB_DEPENDS_DEPENDS_H_
