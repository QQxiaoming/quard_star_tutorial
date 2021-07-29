(:*******************************************************:)
(:Test: op-date-less-than-8                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function used  :)
(:together with "and" expression (le operator).          :)
(:*******************************************************:)
 
(xs:date("2000-01-25Z") le xs:date("2000-10-26Z")) and (xs:date("1975-10-26Z") le xs:date("1975-10-28Z"))