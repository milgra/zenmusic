(ns modules.window-connector
  (:import
    (org.lwjgl.glfw
      GLFW
      GLFWErrorCallback
      GLFWKeyCallback
      GLFWWindowSizeCallback)
    (org.lwjgl.opengl
      GL
      GL11
      GL13
      GL15
      GL20
      GL30)))


(defonce globals (atom {:errorCallback nil
                        :keyCallback   nil
                        :window        nil
                        :width         0
                        :height        0
                        :title         "none"}))


(defn init
  [width height title]

    ;; store properties
  (swap! globals assoc
         :width     width
         :height    height
         :title     title)

  ;; set error callback and store it ( maybe to avoid gc? )
  (swap! globals assoc :errorCallback (GLFWErrorCallback/createPrint System/err))
  (GLFW/glfwSetErrorCallback (:errorCallback @globals))
  ;; init glfw
  (when-not (GLFW/glfwInit) (throw (IllegalStateException. "Unable to initialize GLFW")))

  ;; setup window
  (GLFW/glfwDefaultWindowHints)
  (GLFW/glfwWindowHint GLFW/GLFW_VISIBLE               GLFW/GLFW_TRUE)
  (GLFW/glfwWindowHint GLFW/GLFW_RESIZABLE             GLFW/GLFW_TRUE)
  (GLFW/glfwWindowHint GLFW/GLFW_OPENGL_PROFILE        GLFW/GLFW_OPENGL_CORE_PROFILE)
  (GLFW/glfwWindowHint GLFW/GLFW_OPENGL_FORWARD_COMPAT GL11/GL_TRUE)
  (GLFW/glfwWindowHint GLFW/GLFW_CONTEXT_VERSION_MAJOR 3)
  (GLFW/glfwWindowHint GLFW/GLFW_CONTEXT_VERSION_MINOR 2)
  ;; create window
  (swap! globals assoc :window (GLFW/glfwCreateWindow width height title 0 0))
  (when (= (:window @globals) nil) (throw (RuntimeException. "Failed to create the GLFW window")))
  ;; position window
  (let [vidmode (GLFW/glfwGetVideoMode (GLFW/glfwGetPrimaryMonitor))]
    (GLFW/glfwSetWindowPos
      (:window @globals)
      (/ (- (.width vidmode) width) 2)
      (/ (- (.height vidmode) height) 2))
    (GLFW/glfwMakeContextCurrent (:window @globals))
    (GLFW/glfwSwapInterval 1)
    (GLFW/glfwShowWindow (:window @globals)))
  ;; set key callback  
  (swap! globals assoc
         :keyCallback
         (proxy [GLFWKeyCallback] []
           (invoke
             [window key scancode action mods]
             (when (and (= key GLFW/GLFW_KEY_ESCAPE)
                        (= action GLFW/GLFW_RELEASE))
               (GLFW/glfwSetWindowShouldClose (:window @globals) true)))))

  (swap! globals assoc
         :resizeCallback
         (proxy [GLFWWindowSizeCallback] []
           (invoke
             [window w h]
             (println "resize" w h)
             (swap! globals assoc :width w :height h))))

  (GLFW/glfwSetWindowSizeCallback (:window @globals) (:resizeCallback @globals))
  (GLFW/glfwSetKeyCallback (:window @globals) (:keyCallback @globals)))


(defn destroy
  []


  (.free (:keyCallback @globals))
  (.free (:errorCallback @globals))

  (GLFW/glfwDestroyWindow (:window @globals))
  (GLFW/glfwTerminate))


(defn step
  []

      ;; swap buffer
  (GLFW/glfwSwapBuffers (:window @globals))

    ;; poll events
  (GLFW/glfwPollEvents))


(defn should-close?
  []
  (GLFW/glfwWindowShouldClose (:window @globals)))
