(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-16                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
fn:string((xs:date("1977-12-12Z") - xs:date("1976-12-12Z"))) ge xs:string(xs:dayTimeDuration("P17DT10H02M"))