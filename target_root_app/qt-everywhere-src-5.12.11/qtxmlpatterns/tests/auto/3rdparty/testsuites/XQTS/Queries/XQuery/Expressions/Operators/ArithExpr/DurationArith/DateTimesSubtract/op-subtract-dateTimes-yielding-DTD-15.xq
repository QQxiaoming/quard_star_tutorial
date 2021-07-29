(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-15            :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1978-12-12T10:09:08Z") - xs:dateTime("1977-12-12T09:08:07Z"))) le xs:string(xs:dayTimeDuration("P17DT10H02M"))