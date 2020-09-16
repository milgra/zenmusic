(ns modules.opengl-connector
  (:import
    (java.awt
      Color)
    (java.awt.image
      BufferedImage)
    (java.nio
      ByteBuffer
      FloatBuffer)
    (org.lwjgl
      BufferUtils)
    (org.lwjgl.opengl
      GL
      GL11
      GL13
      GL15
      GL20
      GL30)))


(def vs-shader
  (str "#version 150 core\n"
       "\n"
       "in vec4 in_Position;\n"
       "in vec3 in_Texcoord;\n"
       "uniform float in_Width;\n"
       "uniform float in_Height;\n"
       "out vec3 pass_Texcoord;\n"
       "\n"
       "void main(void) {\n"
       "    gl_Position = vec4( in_Position.x / in_Width , in_Position.y / in_Height , in_Position.z , in_Position.w );\n"
       "    pass_Texcoord = in_Texcoord;\n"
       "}\n"))


(def fs-shader
  (str "#version 150 core\n"
       "\n"
       "uniform sampler2D texture_main;\n"
       "in vec3 pass_Texcoord;\n"
       "out vec4 out_Color;\n"
       "\n"
       "void main(void) {\n"
       "    out_Color = texture2D( texture_main , pass_Texcoord.xy);\n"
       "}\n"))


(defonce globals (atom {;; geom ids
                        :vao-id        0
                        :vbo-id        0
                        ;; shader program ids
                        :vs-id         0
                        :fs-id         0
                        :p-id          0
                        :angle-loc     0}))


(defn load-shader
  [shader-str shader-type]
  (let [shader-id (GL20/glCreateShader shader-type)
        _ (GL20/glShaderSource shader-id shader-str)
        _ (println "init-shaders glShaderSource errors?" (GL11/glGetError) (GL20/glGetShaderInfoLog shader-id))
        _ (GL20/glCompileShader shader-id)
        _ (println "init-shaders glShaderSource errors?" (GL11/glGetError) (GL20/glGetShaderInfoLog shader-id))]
    shader-id))


(defn init
  []

  (let [{:keys [width height]} @globals]
    (GL/createCapabilities)
    (println "OpenGL version:" (GL11/glGetString GL11/GL_VERSION))
    (GL11/glClearColor 0.0 0.0 0.0 0.0)

    ;; buffer init

    (let [vertices (float-array
                     [100.0  0.0   0.0 1.0 0.0
                      0.0    0.0   0.0 1.0 0.0
                      0.0    400.0 0.0 1.0 0.0])
          vertices-buffer (-> (BufferUtils/createFloatBuffer (count vertices))
                              (.put vertices)
                              (.flip))
        ;; create & bind Vertex Array Object
          vao-id (GL30/glGenVertexArrays)
          _ (GL30/glBindVertexArray vao-id)
        ;; create & bind Vertex Buffer Object for vertices
          vbo-id (GL15/glGenBuffers)
          _ (GL15/glBindBuffer GL15/GL_ARRAY_BUFFER vbo-id)
          _ (GL15/glBufferData GL15/GL_ARRAY_BUFFER vertices-buffer GL15/GL_STATIC_DRAW)
          _ (GL20/glVertexAttribPointer 0 2 GL11/GL_FLOAT false 20 0)
          _ (GL20/glVertexAttribPointer 1 3 GL11/GL_FLOAT false 20 8)
          _ (GL15/glBindBuffer GL15/GL_ARRAY_BUFFER 0)
        ;; deselect the VAO
          _ (GL30/glBindVertexArray 0)]
      (swap! globals assoc
             :vao-id vao-id
             :vbo-id vbo-id))

    ;; init shadewrs
    (let [vs-id (load-shader vs-shader GL20/GL_VERTEX_SHADER)
          fs-id (load-shader fs-shader GL20/GL_FRAGMENT_SHADER)
          p-id (GL20/glCreateProgram)
          _ (GL20/glAttachShader p-id vs-id)
          _ (GL20/glAttachShader p-id fs-id)
          _ (GL20/glLinkProgram p-id)

          _ (GL20/glBindAttribLocation p-id 0 "in_Position")
          _ (GL20/glBindAttribLocation p-id 1 "in_Color")

          w-loc (GL20/glGetUniformLocation p-id "in_Width")
          h-loc (GL20/glGetUniformLocation p-id "in_Height")
          _ (println "init-shaders errors?" (GL11/glGetError))

          texture-main-loc (GL20/glGetUniformLocation p-id "texture_main")

          ;; image (flatten (for [i (range (* 512 512))]
          ;;                  [255 0 0 255]))

          image-buffera (BufferUtils/createByteBuffer (* 512 512 4))

          ;; fillup with abgr
          _ (doseq [i (range (* 512 512))]
              (.putInt image-buffera (unchecked-int 0xFF00FFFF)))

          _ (.flip image-buffera)

          image (new BufferedImage 10 80 BufferedImage/TYPE_4BYTE_ABGR)
          _ (.setRGB image 5 20 (.getRGB Color/BLUE))
          ;; image-buffer (-> (BufferUtils/createByteBuffer (* 512 512 4))
          ;;                  (.put (byte-array image))
          ;;                  (.flip))

          tex-id (GL11/glGenTextures)
          _ (GL13/glActiveTexture GL13/GL_TEXTURE0)
          _ (GL11/glBindTexture GL11/GL_TEXTURE_2D tex-id)
          _ (GL11/glPixelStorei GL11/GL_UNPACK_ALIGNMENT 1)
          _ (GL11/glTexImage2D GL11/GL_TEXTURE_2D 0 GL11/GL_RGBA 512 512 0 GL11/GL_RGBA GL11/GL_UNSIGNED_BYTE image-buffera)
          _ (GL30/glGenerateMipmap GL11/GL_TEXTURE_2D)]


      (GL11/glEnable (GL11/GL_TEXTURE_2D))
      (GL11/glEnable (GL11/GL_BLEND))
      (GL11/glBlendFunc GL11/GL_SRC_ALPHA GL11/GL_ONE_MINUS_SRC_ALPHA)

      (swap! globals assoc
             :texture-main-loc texture-main-loc
             :tex-id tex-id
             :vs-id vs-id
             :fs-id fs-id
             :w-loc w-loc
             :h-loc h-loc
             :p-id p-id))))


(defn destroy
  []
  (let [{:keys [p-id vs-id fs-id vao-id vbo-id vboc-id vboi-id]} @globals]
    ;; Delete the shaders
    (GL20/glUseProgram 0)
    (GL20/glDetachShader p-id vs-id)
    (GL20/glDetachShader p-id fs-id)

    (GL20/glDeleteShader vs-id)
    (GL20/glDeleteShader fs-id)
    (GL20/glDeleteProgram p-id)

    ;; Select the VAO
    (GL30/glBindVertexArray vao-id)

    ;; Disable the VBO index from the VAO attributes list
    (GL20/glDisableVertexAttribArray 0)
    (GL20/glDisableVertexAttribArray 1)

    ;; Delete the vertex VBO
    (GL15/glBindBuffer GL15/GL_ARRAY_BUFFER 0)
    (GL15/glDeleteBuffers vbo-id)

    ;; Delete the VAO
    (GL30/glBindVertexArray 0)
    (GL30/glDeleteVertexArrays vao-id)))


(defn draw
  [width height]
  (let [{:keys [w-loc h-loc
                p-id vao-id vbo-id
                texture-main-loc
                indices-count]} @globals]

    (GL11/glClear (bit-or GL11/GL_COLOR_BUFFER_BIT  GL11/GL_DEPTH_BUFFER_BIT))

    (GL20/glUseProgram p-id)
    ;; setup our uniform
    (GL11/glViewport 0 0 width height)
    (GL20/glUniform1i texture-main-loc 0)
    (GL20/glUniform1f w-loc width)
    (GL20/glUniform1f h-loc height)
    ;; Bind to the VAO that has all the information about the
    ;; vertices
    (GL30/glBindVertexArray vao-id)
    (GL20/glEnableVertexAttribArray 0)
    (GL20/glEnableVertexAttribArray 1)
    ;; Bind to the index VBO that has all the information about the
    ;; order of the vertices
    ;;(GL15/glBindBuffer GL15/GL_ELEMENT_ARRAY_BUFFER vboi-id)
    ;; Draw the vertices
    (GL11/glDrawArrays GL11/GL_TRIANGLES 0 3)

    (GL20/glDisableVertexAttribArray 0)
    (GL20/glDisableVertexAttribArray 1)
    (GL30/glBindVertexArray 0)
    (GL20/glUseProgram 0)
    ;;(println "draw errors?" (GL11/glGetError))
    ))


(defn draw1
  [width height vertexes texture]
  (let [{:keys [w-loc h-loc tex-id
                p-id vao-id vbo-id
                texture-main-loc
                indices-count]} @globals]


    (GL11/glClear (bit-or GL11/GL_COLOR_BUFFER_BIT  GL11/GL_DEPTH_BUFFER_BIT))

    ;; upload vertexes
    (.flip vertexes)
    (GL15/glBindBuffer GL15/GL_ARRAY_BUFFER vbo-id)
    (GL15/glBufferData GL15/GL_ARRAY_BUFFER vertexes GL15/GL_STATIC_DRAW)

  ;; upload texture
    (GL11/glBindTexture GL11/GL_TEXTURE_2D tex-id)
    (.clear (:data texture))
    (GL11/glTexImage2D GL11/GL_TEXTURE_2D 0 GL11/GL_RGBA 1024 1024 0 GL11/GL_RGBA GL11/GL_UNSIGNED_BYTE (:data texture))
    (GL30/glGenerateMipmap GL11/GL_TEXTURE_2D)

    (GL20/glUseProgram p-id)
    ;; setup our uniform
    (GL11/glViewport 0 0 width height)
    (GL20/glUniform1i texture-main-loc 0)
    (GL20/glUniform1f w-loc width)
    (GL20/glUniform1f h-loc height)
    ;; Bind to the VAO that has all the information about the
    ;; vertices
    (GL30/glBindVertexArray vao-id)
    (GL20/glEnableVertexAttribArray 0)
    (GL20/glEnableVertexAttribArray 1)
    ;; Bind to the index VBO that has all the information about the
    ;; order of the vertices
    ;;(GL15/glBindBuffer GL15/GL_ELEMENT_ARRAY_BUFFER vboi-id)
    ;; Draw the vertices
    (GL11/glDrawArrays GL11/GL_TRIANGLES 0 (/ (.limit vertexes) 5))

    (GL20/glDisableVertexAttribArray 0)
    (GL20/glDisableVertexAttribArray 1)
    (GL30/glBindVertexArray 0)
    (GL20/glUseProgram 0)))
