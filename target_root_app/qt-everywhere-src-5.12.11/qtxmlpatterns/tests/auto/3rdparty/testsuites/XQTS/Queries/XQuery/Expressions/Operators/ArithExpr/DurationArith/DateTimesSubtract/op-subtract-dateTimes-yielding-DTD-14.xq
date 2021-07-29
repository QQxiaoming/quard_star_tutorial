(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-14            :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1979-12-12T16:16:16Z") - xs:dateTime("1978-12-12T17:17:17Z"))) ne xs:string(xs:dayTimeDuration("P17DT10H02M"))