(:*******************************************************:)
(:Test: op-subtract-yearMonthDurations-6                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 28, 2005                                    :)
(:Purpose: Evaluates The "subtract-yearMonthDurations" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:yearMonthDuration("P02Y09M") - xs:yearMonthDuration("P10Y01M"))