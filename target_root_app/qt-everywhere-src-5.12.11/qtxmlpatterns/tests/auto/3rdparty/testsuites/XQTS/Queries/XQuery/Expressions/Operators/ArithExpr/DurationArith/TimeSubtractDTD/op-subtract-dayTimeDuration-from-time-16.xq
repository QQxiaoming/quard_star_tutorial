(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-time-16         :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of the "subtract-dayTimeDuration-from-time" operator used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
fn:string((xs:time("09:06:07Z") - xs:dayTimeDuration("P18DT02H02M"))) ge  fn:string(xs:time("01:01:01Z"))