(:*******************************************************:)
(:Test: op-multiply-yearMonthDuration-16                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "multiply-yearMonthDuration" operator:)
(:multiplied by +0.  Use of fn:count to avoid empty file. :)
(:*******************************************************:)
 
fn:count((xs:yearMonthDuration("P13Y09M") *+0))