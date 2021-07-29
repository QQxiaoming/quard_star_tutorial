(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-18           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator as :)
(:part of a boolean expression (or operator) and the "fn:false" function. :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1999-10-23T03:02:01Z") - xs:dateTime("1998-09-09T04:04:05Z"))) or fn:false()