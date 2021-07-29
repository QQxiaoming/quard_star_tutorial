(:*******************************************************:)
(:Test: op-gYear-equal-7                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYear-equal" function used     :)
(:together with "and" expression (eq operator).          :)
(:*******************************************************:)
 
(xs:gYear("2000Z") eq xs:gYear("2000Z")) and (xs:gYear("2001Z") eq xs:gYear("2001Z"))