(:*******************************************************:)
(:Test: op-add-yearMonthDurations-11                     :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "add-yearMonthDurations" function used  :)
(:together with a "div" expression.                      :)
(:*******************************************************:)

(xs:yearMonthDuration("P42Y10M") + xs:yearMonthDuration("P28Y10M")) div (xs:yearMonthDuration("P10Y10M") + xs:yearMonthDuration("P60Y10M"))