(:*******************************************************:)
(:Test: op-divide-dayTimeDuration-by-dTD-11              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-dayTimeDuration-by-dTD" operator used  :)
(:together with multiple "div" expressions.              :)
(:*******************************************************:)

fn:round-half-to-even(
(xs:dayTimeDuration("P42DT10H10M") div xs:dayTimeDuration("P10DT10H10M"))
div
(xs:dayTimeDuration("P20DT10H10M") div xs:dayTimeDuration("P18DT10H10M"))
,15)