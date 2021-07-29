(:*******************************************************:)
(:Test: op-subtract-dayTimeDuration-from-time-15         :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-dayTimeDuration-from-time" operator used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
fn:string((xs:time("08:09:09Z") - xs:dayTimeDuration("P17DT10H02M"))) le fn:string(xs:time("09:08:10Z"))