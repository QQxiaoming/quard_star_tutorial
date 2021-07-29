(:*******************************************************:)
(:Test: op-subtract-yearMonthDurations-11                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 28, 2005                                    :)
(:Purpose: Evaluates The "subtract-yearMonthDurations" function used  :)
(:together with a "div" expression.                      :)
(:*******************************************************:)

(xs:yearMonthDuration("P42Y10M") - xs:yearMonthDuration("P20Y10M")) div (xs:yearMonthDuration("P20Y11M") - xs:yearMonthDuration("P18Y11M"))