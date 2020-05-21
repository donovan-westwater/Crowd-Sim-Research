Shader "Unlit/FloorDirection"
{
    Properties
    {
        _BoundColor("Bound Color", Color) = (1,1,1,1)
        _BgColor("Background Color", Color) = (1,1,1,1)
        _MainTex("Texture", 2D) = "red" {}
        _BoundWidth("BoundWidth", float) = 10
        _ComponentWidth("ComponentWidth", float) = 100
        _AgentPosX("AgentPosX", float) = 0.5
        _AgentPosY("AgentPosY", float) = 0.5
        _Radius("Radius", float) = 0.1
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100

        
            CGPROGRAM
            //#pragma vertex vert
            //#pragma fragment frag
            // make fog work
            #pragma multi_compile_fog
            #pragma surface surf Standard fullforwardshadows

            float _BoundWidth;
            fixed4 _BoundColor;
            fixed4 _BgColor;
            float _ComponentWidth;
            float  _AgentPosX;
            float  _AgentPosY;
            float _Radius;
            float3 _aLocs[300];

            struct appdata
            {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
            };
            /*
            struct v2f
            {
                float2 uv : TEXCOORD0;
                UNITY_FOG_COORDS(1)
                float4 vertex : SV_POSITION;
            };
            */
            sampler2D _MainTex;
            //float4 _MainTex_ST;

            struct Input
            {
                float2 uv_MainTex;	// The UV of the terrain texture
                float3 worldPos;	// The in-world position
            };

            void surf(Input IN, inout SurfaceOutputStandard o)
            {
                //fixed4 c = tex2D(_MainTex,i.uv);
                float x = IN.worldPos.x;
                float y = IN.worldPos.y;
                o.Albedo = tex2D(_MainTex, IN.uv_MainTex).rgb;
                for(int i = 0; i < 1000;i++){
                    float3 c = _aLocs[i];//{ _AgentPosX, 0,_AgentPosY };
                    if (c.y == 0) break;
                    c.y = 0;
                    float dis = distance(c, IN.worldPos);//sqrt(pow((_AgentPosX - x), 2) + pow((_AgentPosY- y), 2));
                    if (dis < _Radius) { 
                        float innerRadius = (_ComponentWidth * _Radius - _BoundWidth) / _ComponentWidth; //0.1 == radis
                        if (dis > innerRadius) {
                            o.Albedo = _BoundColor;
                            //c.a = c.a*antialias(_BoundWidth, dis, innerRadius);
                        }
                        else {
                            o.Albedo = _BgColor;
                        }
                    }
                }
            }
            ENDCG
        
    }
}
