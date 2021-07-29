(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-15                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
fn:string((xs:date("1978-12-12Z") - xs:date("1977-03-12Z"))) le xs:string(xs:dayTimeDuration("P17DT10H02M"))