(ns modules.compositor
  (:require
    #?(:clj
       [modules.opengl-connector :as gl]
       :cljs
       [modules.webgl-connector :as gl])
    [utils.bitmap :as bm]
    [utils.texmap :as tm])
  (:import
    (java.nio
      ByteBuffer)
    (org.lwjgl
      BufferUtils)))


(def state
  (atom {:rects {}
         :texes {}
         :texmap (tm/create 1024 1024)
         :texture (-> (bm/create 1024 1024)
                      (bm/fill 0xFFFF00FF))}))


(defn init!
  "inits the compositor"
  []
  (swap! state assoc-in [:texes :tex1] (bm/create 1024 1024)))


(defn create-rect
  "creates new rectangle"
  [{:keys [id bmp x y w h d] :as rect}]
  (println "create rect" id)
  ;; store rect
  (swap! state assoc-in [:rects id] rect)
  ;; add bitmap to texture
  (bm/insert (:texture @state) bmp (int x) (int y)))


(defn delete-rect
  "deletes rectangle"
  [id]
  (println "delete rect" id)
  (swap! state update-in [:rects] dissoc id))


(defn update-rect
  "update rectangle"
  [{:keys [id bmp x y w h d]}]
  (println "update rect" id)
  (swap! state assoc-in [:rects id] {:bmp bmp
                                     :x x
                                     :y y
                                     :w w
                                     :h h
                                     :d d
                                     :changed true}))


(defn render
  "render rectangles"
  [w h]
  (let [vertices (float-array
                   [500.0  0.0   1.0 0.0 0.0
                    0.0    0.0   0.0 0.0 0.0
                    0.0    500.0 0.0 1.0 0.0
                    0.0    500.0 0.0 1.0 0.0
                    500.0  500.0 1.0 1.0 0.0
                    500.0  0.0   1.0 0.0 0.0])
        vertices-buffer (-> (BufferUtils/createFloatBuffer (count vertices))
                            (.put vertices)
                            (.flip))]
    (gl/draw1 w h vertices-buffer (:texture @state))


  ;; iterate through all rectangles
  ;; build up vertex bytebuffer
  ;; update texture maps
  ;; draw
    ))
