(:*******************************************************:)
(:Test: op-gYear-equal-8                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYear-equal" function used     :)
(:together with "and" expression (ne operator).          :)
(:*******************************************************:)
 
(xs:gYear("2000Z") ne xs:gYear("2000Z")) and (xs:gYear("1975Z") ne xs:gYear("1975Z"))