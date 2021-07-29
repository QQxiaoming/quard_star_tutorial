(:*******************************************************:)
(:Test: op-gMonthDay-equal-2                             :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function      :)
(:As per example 2 (for this function) of the F&O  specs :)
(:*******************************************************:)

(xs:gMonthDay("--12-25Z") eq xs:gMonthDay("--12-26Z")) 