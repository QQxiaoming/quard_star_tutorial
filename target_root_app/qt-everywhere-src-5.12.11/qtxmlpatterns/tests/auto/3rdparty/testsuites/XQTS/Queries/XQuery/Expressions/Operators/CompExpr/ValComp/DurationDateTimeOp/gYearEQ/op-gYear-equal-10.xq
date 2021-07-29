(:*******************************************************:)
(:Test: op-gYear-equal-10                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYear-equal" function used     :)
(:together with "or" expression (ne operator).           :)
(:*******************************************************:)
 
(xs:gYear("1976Z") ne xs:gYear("1976Z")) or (xs:gYear("1980Z") ne xs:gYear("1980Z"))