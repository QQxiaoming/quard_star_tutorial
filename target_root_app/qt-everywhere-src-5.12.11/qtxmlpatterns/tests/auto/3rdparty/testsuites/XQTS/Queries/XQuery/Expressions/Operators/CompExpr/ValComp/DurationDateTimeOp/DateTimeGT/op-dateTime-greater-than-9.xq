(:*******************************************************:)
(:Test: op-dateTime-greater-than-9                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-greater-than" function used:)
(:together with "or" expression (gt operator).           :)
(:*******************************************************:)
 
(xs:dateTime("2002-06-02T12:00:10Z") gt xs:dateTime("2000-04-04T12:00:00Z")) or (xs:dateTime("2002-04-02T13:00:10Z") gt xs:dateTime("2001-04-02T10:00:00Z"))