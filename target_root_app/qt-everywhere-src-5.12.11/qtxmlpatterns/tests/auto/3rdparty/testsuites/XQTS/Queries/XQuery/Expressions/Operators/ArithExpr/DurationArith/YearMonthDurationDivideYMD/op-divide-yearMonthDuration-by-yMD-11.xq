(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-11            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator used  :)
(:together with multiple "div" expressions.              :)
(:*******************************************************:)

round-half-to-even(
  (xs:yearMonthDuration("P42Y10M") div xs:yearMonthDuration("P20Y10M"))
    div 
  (xs:yearMonthDuration("P20Y11M") div xs:yearMonthDuration("P18Y11M")),
15)