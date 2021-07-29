(:*******************************************************:)
(:Test: op-gYear-equal-1                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYear-equal" function          :)
(:As per example 1 (for this function)of the F&O specs   :)
(:*******************************************************:)

(xs:gYear("2005-12:00") eq xs:gYear("2005+12:00"))
