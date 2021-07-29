(:*******************************************************:)
(:Test: op-date-greater-than-9                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-greater-than" function used:)
(:together with "or" expression (gt operator).           :)
(:*******************************************************:)
 
(xs:date("2000-10-26Z") gt xs:date("2000-10-28Z")) or (xs:date("1976-10-28Z") gt xs:date("1976-10-28Z"))