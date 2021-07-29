(:*******************************************************:)
(:Test: op-date-greater-than-7                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "date-greater-than" function used:)
(:together with "and" expression (gt operator).          :)
(:*******************************************************:)
 
(xs:date("2000-01-01Z") gt xs:date("2000-01-01Z")) and (xs:date("2001-02-02Z") gt xs:date("2001-03-02Z"))