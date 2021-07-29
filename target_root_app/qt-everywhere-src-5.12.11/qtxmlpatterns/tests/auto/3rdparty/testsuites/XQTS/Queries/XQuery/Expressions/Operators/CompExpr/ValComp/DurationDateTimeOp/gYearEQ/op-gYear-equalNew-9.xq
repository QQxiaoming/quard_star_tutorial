(:*******************************************************:)
(:Test: op-gYear-equal-9                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYear-equal" function used     :)
(:together with "or" expression (eq operator).           :)
(:*******************************************************:)
 
(xs:gYear("2000Z") eq xs:gYear("2000Z")) or (xs:gYear("1976Z") eq xs:gYear("1976Z"))