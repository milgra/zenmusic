(require 'leiningen.core.eval)

;; per-os jvm-opts code cribbed from Overtone
(def JVM-OPTS
  {:common   []
   :macosx   ["-XstartOnFirstThread" "-Djava.awt.headless=true"]
   :linux    []
   :windows  []})


(defn jvm-opts
  "Return a complete vector of jvm-opts for the current os."
  []
  (let [os (leiningen.core.eval/get-os)]
    (vec (set (concat (get JVM-OPTS :common)
                      (get JVM-OPTS os))))))


(def LWJGL_NS "org.lwjgl")

;; Edit this to change the version.
(def LWJGL_VERSION "3.1.5")

;; Edit this to add/remove packages.
(def LWJGL_MODULES
  ["lwjgl"
   "lwjgl-assimp"
   "lwjgl-bgfx"
   "lwjgl-egl"
   "lwjgl-glfw"
   "lwjgl-jawt"
   "lwjgl-jemalloc"
   "lwjgl-lmdb"
   "lwjgl-lz4"
   "lwjgl-nanovg"
   "lwjgl-nfd"
   "lwjgl-nuklear"
   "lwjgl-odbc"
   "lwjgl-openal"
   "lwjgl-opencl"
   "lwjgl-opengl"
   "lwjgl-opengles"
   "lwjgl-openvr"
   "lwjgl-par"
   "lwjgl-remotery"
   "lwjgl-rpmalloc"
   "lwjgl-sse"
   "lwjgl-stb"
   "lwjgl-tinyexr"
   "lwjgl-tinyfd"
   "lwjgl-tootle"
   "lwjgl-vulkan"
   "lwjgl-xxhash"
   "lwjgl-yoga"
   "lwjgl-zstd"])

;; It's safe to just include all native dependencies, but you might
;; save some space if you know you don't need some platform.
(def LWJGL_PLATFORMS ["linux" "macos" "windows"])

;; These packages don't have any associated native ones.
(def no-natives?
  #{"lwjgl-egl" "lwjgl-jawt" "lwjgl-odbc"
    "lwjgl-opencl" "lwjgl-vulkan"})


(defn lwjgl-deps-with-natives
  []
  (apply concat
         (for [m LWJGL_MODULES]
           (let [prefix [(symbol LWJGL_NS m) LWJGL_VERSION]]
             (into [prefix]
                   (if (no-natives? m)
                     []
                     (for [p LWJGL_PLATFORMS]
                       (into prefix [:classifier (str "natives-" p)
                                     :native-prefix ""]))))))))


(def all-dependencies
  (into ;; Add your non-LWJGL dependencies here
   '[[org.clojure/clojure "1.8.0"]
     [cider/cider-nrepl "0.25.3"]
     [nrepl "0.8.1"]]
   (lwjgl-deps-with-natives)))


(defproject zen_music "0.4.0"
  :description "Simple LWJGL3 clojure test."
  :url "https://github.com/rogerallen/hello_lwjgl"
  :license {:name "Eclipse Public License"
            :url "http://www.eclipse.org/legal/epl-v10.html"}
  :dependencies ~all-dependencies
  :min-lein-version "2.1.0"
  :jvm-opts ^:replace ~(jvm-opts)
  :main zen-music.core)
