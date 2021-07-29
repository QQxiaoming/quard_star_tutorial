(:*******************************************************:)
(:Test: op-date-less-than-7                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-less-than" function used  :)
(:together with "and" expression (lt operator).          :)
(:*******************************************************:)
 
(xs:date("2000-01-01Z") lt xs:date("2000-01-01Z")) and (xs:date("2001-02-02Z") lt xs:date("2001-03-02Z"))