////////////////////////////////////////////////////////////////////////
// Matrix multiplication utilities. Help facilitate cross compilation.
float3 PsyMatMul( float3 Row0, float3 Row1, float3 Row2, float3 Vector )
{
	float3 RetVal = float3( 
		dot( Vector.xyz, Row0.xyz ),
		dot( Vector.xyz, Row1.xyz ),
		dot( Vector.xyz, Row2.xyz ) );
	return RetVal;
}

float4 PsyMatMul( float4 Row0, float4 Row1, float4 Row2, float4 Row3, float4 Vector )
{
	float4 RetVal = float4( 
		dot( Vector.xyzw, Row0 ),
		dot( Vector.xyzw, Row1 ),
		dot( Vector.xyzw, Row2 ),
		dot( Vector.xyzw, Row3 ) );
	return RetVal;
}

float3 PsyMatMul( float3x3 Mat, float3 Vector )
{
	return mul( Mat, Vector );
}

float4 PsyMatMul( float4x4 Mat, float4 Vector )
{
	return mul( Mat, Vector );
}

float3 PsyMatMulTranspose( float3 Row0, float3 Row1, float3 Row2, float3 Vector )
{
	float3 RetVal = Vector.xxx * Row0;
	RetVal += Vector.yyy * Row1;
	RetVal += Vector.zzz * Row2;
	return RetVal;
}

float4 PsyMatMulTranspose( float4 Row0, float4 Row1, float4 Row2, float4 Row3, float4 Vector )
{
	float4 RetVal = Vector.xxxx * Row0;
	RetVal += Vector.yyyy * Row1;
	RetVal += Vector.zzzz * Row2;
	RetVal += Vector.wwww * Row3;
	return RetVal;
}
