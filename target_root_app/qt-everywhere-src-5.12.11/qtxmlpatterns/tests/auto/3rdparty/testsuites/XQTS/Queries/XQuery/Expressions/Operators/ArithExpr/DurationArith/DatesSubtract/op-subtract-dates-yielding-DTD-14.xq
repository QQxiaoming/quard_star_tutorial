(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-14                :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
fn:string((xs:date("1979-12-12Z") - xs:date("1979-11-11Z"))) ne xs:string(xs:dayTimeDuration("P17DT10H02M"))