(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-12            :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator used :)
(:with a boolean expression and the "fn:true" function.   :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1980-03-02T11:11:11Z") - xs:dateTime("1981-12-12T12:12:12Z"))) and (fn:true())