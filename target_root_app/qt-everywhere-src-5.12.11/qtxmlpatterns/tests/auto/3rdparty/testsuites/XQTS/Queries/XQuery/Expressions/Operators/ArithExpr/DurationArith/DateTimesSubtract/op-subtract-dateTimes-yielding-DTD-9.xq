(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-9             :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator used  :)
(:together with an "and" expression.                      :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1993-12-09T04:04:04Z") - xs:dateTime("1992-12-09T05:05:05Z"))) and fn:string((xs:dateTime("1993-12-09T01:01:01Z") - xs:dateTime("1992-12-09T06:06:06Z")))