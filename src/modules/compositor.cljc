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
                      (bm/fill 0xFFFF00FF))
         :did-change? false
         :vertexes (BufferUtils/createFloatBuffer 5000)}))


(defn init!
  "inits the compositor"
  []
  (swap! state assoc-in [:texes :tex1] (bm/create 1024 1024)))


(defn create-rect
  "creates new rectangle"
  [{:keys [id bmp x y w h d] :as rect}]

  (let [newtexmap (tm/add-bitmap (:texmap @state) id bmp)
        newcoords (tm/get-coords newtexmap id)
        newrect (assoc rect :coords newcoords)]
    (swap! state assoc-in [:rects id] newrect)
    (swap! state assoc :texmap newtexmap)
    (swap! state assoc :did-change? true)))


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
  (.clear (:vertexes @state))

  (doseq [rect (vals (:rects @state))]
    (let [{:keys [x y w h coords]} rect
          [tlx tly brx bry] coords
          vertexes
          [x       y       tlx tly 0.0
           (+ x w) y       brx tly 0.0
           x       (+ y h) tlx bry 0.0

           (+ x w) y       brx tly 0.0
           (+ x w) (+ y h) brx bry 0.0
           x       (+ y h) tlx bry 0.0]

          vertexes1 [500.0  0.0   1.0 0.0 0.0
                     0.0    0.0   0.0 0.0 0.0
                     0.0    500.0 0.0 1.0 0.0
                     0.0    500.0 0.0 1.0 0.0
                     500.0  500.0 1.0 1.0 0.0
                     500.0  0.0   1.0 0.0 0.0]

          float-vertexes (float-array vertexes)]

      (.put (:vertexes @state) float-vertexes)))

  (gl/draw1 w h (:vertexes @state) (:bitmap (:texmap @state))))
