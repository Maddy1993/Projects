;; The first three lines of this file were inserted by DrRacket. They record metadata
;; about the language level of this file in a form that our tools can easily process.
#reader(lib "htdp-intermediate-lambda-reader.ss" "lang")((modname q1) (read-case-sensitive #t) (teachpacks ()) (htdp-settings #(#t constructor repeating-decimal #f #t none #f () #f)))
(require rackunit)
(require "extras.rkt")
(check-location "01" "q1.rkt")

(provide pyramid-volume)

;; Volume of Pyramid

;; Data Definitions:
;; A Pyramid is a three-face figure with a circular or square base.
;; Volume of Pyramid can be calculated using the height and the length/ radius of square/ circle.
;; x : NonNegInt  is the base unit of the Pyramid, measured in meteres
;; h : NonNegInt  is the height unit of the Pyramid, measures in meters

;; pyramid-volume: x h -> Volume
;; GIVEN: We input the base and height values of the given pyramid.
;; RETURNS: it returns the calcualted volume of the pyramid using the given base and height values.

;; EXAMPLES:
;; (pyramid-volume 2 5) = 20/3
;; (pyramid-volume 3 6) = 18
;; DESIGN STRATEGY: Transcribe the Formula


(define (pyramid-volume x h)
  (* (/ 1 3) x x h))

;; TESTS:
(begin-for-test
  (check-equal? (pyramid-volume 2 5) 20/3))
