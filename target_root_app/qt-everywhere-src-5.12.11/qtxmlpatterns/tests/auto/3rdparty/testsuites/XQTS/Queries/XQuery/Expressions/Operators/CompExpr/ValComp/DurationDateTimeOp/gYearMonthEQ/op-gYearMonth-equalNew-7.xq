(:*******************************************************:)
(:Test: op-gYearMonth-equal-7                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function used:)
(:together with "and" expression (eq operator).          :)
(:*******************************************************:)
 
(xs:gYearMonth("2000-02Z") eq xs:gYearMonth("2000-03Z")) and (xs:gYearMonth("2001-01Z") eq xs:gYearMonth("2001-01Z"))