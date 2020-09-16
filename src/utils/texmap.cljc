(ns utils.texmap
  (:require
    [utils.bitmap :as bm]))


(defn create
  [width height]
  "create texture map"
  {:bitmap (->
             (bm/create width height)
             (bm/fill 0x2200FFFF))
   :coords {}
   :cursor [0 0 0]
   :is-full? false
   :did-change? true})


(defn reset
  "reset texture map"
  [texmap]
  (assoc texmap
         :bitmap (bm/fill (:bitmap texmap) 0)
         :coords {}
         :cursor [0 0 0]
         :is-full? false
         :did-change? true))


(defn get-coords
  "returns coords for given bitmap id"
  [texmap id]
  (let [w (:width (:bitmap texmap))
        h (:height (:bitmap texmap))
        [tlx tly brx bry] (id (:coords texmap))]
    [(/ tlx w) (/ tly h) (/ brx w) (/ bry h)]))


(defn add-bitmap
  "adds bitmap to texture map with given id"
  [texmap id bitmap]
  (let [{:keys [width height]} bitmap
        [cx cy ch] (:cursor texmap)
        tex-bitmap (:bitmap texmap)
        tex-width (:width tex-bitmap)
        tex-height (:height tex-bitmap)

        ;; set cursor height
        nch (if (> (+ cx width) tex-width)
              height
              (if (> height ch)
                height
                ch))

        ;; set cursor y position
        ncy (if (> (+ cx width) tex-width)
              (+ cy ch)
              cy)

        ;; set cursor x position
        ncx (if (> (+ cx width) tex-width)
              0
              cx)

        ;; right bottom coords
        rbx  (+ ncx width)
        rby  (+ ncy height)

        ;; texture map is full, needs cleanup
        is-full? (> nch tex-height)]

    (if is-full?
      (assoc texmap :is-full? true)
      (-> texmap
          (assoc :bitmap (bm/insert tex-bitmap bitmap ncx ncy))
          (assoc-in [:coords id] [ncx ncy rbx rby])
          (assoc :cursor [rbx ncy nch])
          (assoc :did-change? true)))))


(defn update-bitmap
  [texmap id bitmap]
  (let [[ocx ocy _ _ :as coords] (get-coords texmap id)
        tex-bitmap (:bitmap texmap)]
    (if coords
      ;; update bitmap at position
      (-> texmap
          (assoc :bitmap (bm/insert tex-bitmap bitmap ocx ocy))
          (assoc :did-change? true)))))
