(ns zen-music.core
  (:gen-class)
  (:require
    [clojure.pprint :as pprint]
    [modules.compositor :as co]
    [modules.opengl-connector :as gl]
    [modules.window-connector :as wc]
    [utils.bitmap :as bitmap])
  (:import
    (org.lwjgl
      Version)))

;; ======================================================================
(defn start-cider-nrepl
  []
  (.start (Thread. (fn []
                     (println "Starting Cider Nrepl Server Port 7888")
                     (load-string (str "(require '[nrepl.server :as nrepl-server])"
                                       "(require '[cider.nrepl :as cider])"
                                       "(nrepl-server/start-server :port 7888 :handler cider/cider-nrepl-handler)"))))))


(defonce globals (atom {:angle         0.0
                        :last-time     0}))


(defn update-globals
  []
  (let [{:keys [angle last-time]} @globals
        cur-time (System/currentTimeMillis)
        delta-time (- cur-time last-time)
        next-angle (+ (* delta-time 0.05) angle)
        next-angle (if (>= next-angle 360.0)
                     (- next-angle 360.0)
                     next-angle)]
    (swap! globals assoc
           :angle next-angle
           :last-time cur-time)))


(defn gen-id!
  [length]
  "generates fixed length alfanumeric hash"
  (let [chars (map char (concat (range 48 57) (range 65 90) (range 97 122)))
        id (take length (repeatedly #(rand-nth chars)))]
    (keyword (reduce str id))))


(defn main-loop
  []
  (let [rects (map (fn [_]
                     {:id  (gen-id! 5)
                      :x (rand 950.0)
                      :y (rand 950.0)
                      :w 50.0
                      :h 50.0
                      :d 1
                      :bmp (-> (bitmap/create 50 50)
                               (bitmap/fill (bit-or 0xFF000000 (rand-int 0xFFFFFF))))}) (range 5))]

    (doseq [rect rects]
      (co/create-rect rect))

    (while (not (wc/should-close?))
      (update-globals)
      ;;(gl/draw (:width @wc/globals) (:height @wc/globals))
      (co/render (:width @wc/globals) (:height @wc/globals))
      (wc/step))))


(defn -main
  [& args]
  (println "Hello, Lightweight Java Game Library! V" (Version/getVersion) args)
  (when (= "cider" (second args))
    (start-cider-nrepl))

  (co/init!)
  (swap! globals assoc
         :last-time (System/currentTimeMillis))

  (try
    (wc/init 800 600 "delta")
    (gl/init)
    (main-loop)
    (finally
      (gl/destroy))))
