(:*******************************************************:)
(:Test: op-date-greater-than-8                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-greater-than" function used:)
(:together with "and" expression (ge operator).          :)
(:*******************************************************:)
 
(xs:date("2000-01-25Z") ge xs:date("2000-10-26Z")) and (xs:date("1975-10-26Z") ge xs:date("1975-10-28Z"))