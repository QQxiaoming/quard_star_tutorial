(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-13            :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator used  :)
(:together with the numeric-equal-operator "eq".         :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1980-05-05T13:13:13Z") - xs:dateTime("1979-10-05T14:14:14Z"))) eq xs:string(xs:dayTimeDuration("P17DT10H02M"))