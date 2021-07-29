(:*******************************************************:)
(:Test: op-date-less-than-9                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function used  :)
(:together with "or" expression (lt operator).           :)
(:*******************************************************:)
 
(xs:date("2000-10-26Z") lt xs:date("2000-10-28Z")) or (xs:date("1976-10-28Z") lt xs:date("1976-10-28Z"))