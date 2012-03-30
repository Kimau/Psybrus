/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.40
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;
using System.Runtime.InteropServices;

public class DatabaseFilter : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal DatabaseFilter(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(DatabaseFilter obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~DatabaseFilter() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          RakNetPINVOKE.delete_DatabaseFilter(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

  public void Serialize(BitStream arg0) {
    RakNetPINVOKE.DatabaseFilter_Serialize(swigCPtr, BitStream.getCPtr(arg0));
  }

  public bool Deserialize(BitStream arg0) {
    bool ret = RakNetPINVOKE.DatabaseFilter_Deserialize(swigCPtr, BitStream.getCPtr(arg0));
    return ret;
  }

  public Cell cellValue {
    set {
      RakNetPINVOKE.DatabaseFilter_cellValue_set(swigCPtr, Cell.getCPtr(value));
    } 
    get {
      IntPtr cPtr = RakNetPINVOKE.DatabaseFilter_cellValue_get(swigCPtr);
      Cell ret = (cPtr == IntPtr.Zero) ? null : new Cell(cPtr, false);
      return ret;
    } 
  }

  public Table.FilterQueryType operation {
    set {
      RakNetPINVOKE.DatabaseFilter_operation_set(swigCPtr, (int)value);
    } 
    get {
      Table.FilterQueryType ret = (Table.FilterQueryType)RakNetPINVOKE.DatabaseFilter_operation_get(swigCPtr);
      return ret;
    } 
  }

  public Table.ColumnType columnType {
    set {
      RakNetPINVOKE.DatabaseFilter_columnType_set(swigCPtr, (int)value);
    } 
    get {
      Table.ColumnType ret = (Table.ColumnType)RakNetPINVOKE.DatabaseFilter_columnType_get(swigCPtr);
      return ret;
    } 
  }

  public string columnName {
    set {
      RakNetPINVOKE.DatabaseFilter_columnName_set(swigCPtr, value);
    } 
    get {
      string ret = RakNetPINVOKE.DatabaseFilter_columnName_get(swigCPtr);
      return ret;
    } 
  }

  public DatabaseFilter() : this(RakNetPINVOKE.new_DatabaseFilter(), true) {
  }

}
