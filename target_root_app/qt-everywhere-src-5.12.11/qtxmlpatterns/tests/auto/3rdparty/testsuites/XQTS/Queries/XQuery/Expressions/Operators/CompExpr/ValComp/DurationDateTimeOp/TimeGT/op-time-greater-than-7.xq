(:*******************************************************:)
(:Test: op-time-greater-than-7                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "time-greater-than" function used:)
(:together with "and" expression (gt operator).          :)
(:*******************************************************:)
 
(xs:time("13:00:00Z") gt xs:time("17:00:00Z")) and (xs:time("13:00:00Z") gt xs:time("17:00:00Z"))