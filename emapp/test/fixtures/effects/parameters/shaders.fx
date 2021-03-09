float4
MainVS() : POSITION
{
    return float4(0);
}

float4
MainPS() : COLOR
{
    return float4(0);
}

technique T {
    pass P {
        VertexShader = compile vs_3_0 MainVS();
        PixelShader  = compile ps_3_0 MainPS();
    }
}
