(:*******************************************************:)
(:Test: op-gYearMonth-equal-8                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function used:)
(:together with "and" expression (ne operator).          :)
(:*******************************************************:)
 
(xs:gYearMonth("2000-01Z") ne xs:gYearMonth("2000-01Z")) and (xs:gYearMonth("1975-01Z") ne xs:gYearMonth("1975-03Z"))