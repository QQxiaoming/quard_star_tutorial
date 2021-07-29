(:*******************************************************:)
(:Test: op-date-greater-than-10                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-greater-than" function used  :)
(:together with "or" expression (ge operator).           :)
(:*******************************************************:)
 
(xs:date("1976-10-25Z") ge xs:date("1976-10-28Z")) or (xs:date("1980-08-11Z") ge xs:date("1980-08-10Z"))