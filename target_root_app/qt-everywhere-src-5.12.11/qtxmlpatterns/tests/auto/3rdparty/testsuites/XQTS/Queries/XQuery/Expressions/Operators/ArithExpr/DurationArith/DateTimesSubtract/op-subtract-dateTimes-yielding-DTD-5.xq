(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-5             :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator that  :)
(:is used as an argument to the fn:boolean function.     :)
(:*******************************************************:)
 
fn:boolean(fn:string(xs:dateTime("1962-03-12T10:09:09Z") - xs:dateTime("1961-02-01T20:10:10Z")))