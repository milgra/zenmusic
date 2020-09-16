(ns utils.bitmap
  (:import
    (java.nio
      ByteBuffer)
    (org.lwjgl
      BufferUtils)))


(defn create
  "create bitmap"
  [w h]
  (let [data (BufferUtils/createByteBuffer (* w h 4))]
    {:width w
     :height h
     :data data}))


(defn fill
  "fill with color"
  [bitmap color]
  (let [{:keys [width height data]} bitmap]
    (dotimes [i (* width height)]
      (.putInt data (unchecked-int color)))
    bitmap))


(defn insert
  "insert bitmap into a larger bitmap"
  [src tgt x y]
  (let [{ds :data ws :width hs :height} src
        {dt :data wt :width ht :height} tgt
        wcnt (min ws (- wt x))  ;; maximum horizontal pixels
        hcnt (min hs (- ht y))] ;; maximum vertical pixels
    (dotimes [row hcnt]
      (let [sind (* row ws 4)                     ;; source index is at the beginning of its rows
            tind (* (+ (* (+ y row) wt) x) 4)]    ;; target index depends on x position

        (.limit dt (+ tind (* wcnt 4))) ;; limit target buffer to row end
        (.position dt tind) ;; set target buffer position to index

        (.limit ds (+ sind (* wcnt 4))) ;; limit target buffer to row end
        (.position ds sind) ;; set target buffer position to index

        (.put dt ds) ;; copy source
        ))))

