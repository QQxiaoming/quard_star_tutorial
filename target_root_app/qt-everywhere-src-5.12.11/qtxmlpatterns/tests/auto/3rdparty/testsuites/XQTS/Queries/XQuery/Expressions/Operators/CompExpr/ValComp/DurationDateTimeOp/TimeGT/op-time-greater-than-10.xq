(:*******************************************************:)
(:Test: op-time-greater-than-10                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "time-greater-than" function used:)
(:together with "or" expression (ge operator).          :)
(:*******************************************************:)
 
(xs:time("13:00:00Z") ge xs:time("17:00:00Z")) or (xs:time("13:00:00Z") ge xs:time("17:00:00Z"))