(:*******************************************************:)
(:Test: op-gYearMonth-equal-9                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function used:)
(:together with "or" expression (eq operator).           :)
(:*******************************************************:)
 
(xs:gYearMonth("2000-01Z") eq xs:gYearMonth("2000-03Z")) or (xs:gYearMonth("1976-06Z") eq xs:gYearMonth("1976-06Z"))