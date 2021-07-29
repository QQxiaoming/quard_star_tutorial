(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-13                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator used  :)
(:together with the numeric-equal-operator "eq".         :)
(:*******************************************************:)
 
fn:string((xs:date("1980-05-05Z") - xs:date("1981-12-03Z"))) eq xs:string(xs:dayTimeDuration("P17DT10H02M"))