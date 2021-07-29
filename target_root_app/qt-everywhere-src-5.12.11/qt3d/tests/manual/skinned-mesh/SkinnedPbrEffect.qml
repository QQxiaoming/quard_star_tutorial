import Qt3D.Core 2.10
import Qt3D.Render 2.10
import Qt3D.Input 2.0
import Qt3D.Extras 2.10

Effect {
    id: skinnedPbrEffect
    parameters: [
        Parameter { name: "baseColor"; value: "red" },
        Parameter { name: "metalness"; value: 0.1 },
        Parameter { name: "roughness"; value: 0.2 }
    ]

    techniques: [
        Technique {
            filterKeys: FilterKey { name: "renderingStyle"; value: "forward" }

            graphicsApiFilter {
                api: GraphicsApiFilter.OpenGL
                majorVersion: 3
                minorVersion: 1
                profile: GraphicsApiFilter.CoreProfile
            }

            renderPasses: RenderPass {
                shaderProgram: ShaderProgram {
                    id: prog
                    vertexShaderCode: loadSource("qrc:/skinnedPbr.vert")
                }

                ShaderProgramBuilder {
                    shaderProgram: prog
                    fragmentShaderGraph: "qrc:/shaders/graphs/metalrough.frag.json"
                    enabledLayers: ["baseColor", "metalness", "roughness", "ambientOcclusion", "normal"]
                }
            }
        }
    ]
}
