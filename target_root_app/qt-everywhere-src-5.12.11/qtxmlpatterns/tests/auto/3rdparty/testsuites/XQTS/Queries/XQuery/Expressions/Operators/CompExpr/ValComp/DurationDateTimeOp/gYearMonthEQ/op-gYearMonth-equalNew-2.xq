(:*******************************************************:)
(:Test: op-gYearMonth-equal-2                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "gYearMonth-equal" function     :)
(:As per example 2 (for this function) of the F&O  specs :)
(:*******************************************************:)

(xs:gYearMonth("1976-03-05:00") eq xs:gYearMonth("1976-03Z"))