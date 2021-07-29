(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-4             :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator that  :)
(:return true and used together with fn:not.             :)
(:*******************************************************:)
 
fn:not(fn:string(xs:dateTime("1998-09-12T11:12:12Z") - xs:dateTime("1996-02-02T01:01:01Z")))