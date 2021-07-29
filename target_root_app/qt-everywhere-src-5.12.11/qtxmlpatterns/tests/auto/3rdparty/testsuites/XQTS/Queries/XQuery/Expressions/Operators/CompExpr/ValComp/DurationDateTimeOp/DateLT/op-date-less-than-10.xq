(:*******************************************************:)
(:Test: op-date-less-than-10                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function used  :)
(:together with "or" expression (le operator).           :)
(:*******************************************************:)
 
(xs:date("1976-10-25Z") le xs:date("1976-10-28Z")) or (xs:date("1980-08-11Z") le xs:date("1980-08-10Z"))