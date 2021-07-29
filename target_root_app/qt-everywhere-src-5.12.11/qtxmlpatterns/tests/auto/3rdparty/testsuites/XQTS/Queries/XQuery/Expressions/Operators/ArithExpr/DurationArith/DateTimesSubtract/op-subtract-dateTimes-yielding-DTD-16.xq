(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-16             :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1977-12-12T01:02:02Z") - xs:dateTime("1976-12-12T02:03:04Z"))) ge  xs:string(xs:dayTimeDuration("P18DT02H02M"))