(:*******************************************************:)
(: Test: K2-SeqExprCast-480                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: All primites in one query.                   :)
(:*******************************************************:)
xs:untypedAtomic("xs:untypedAtomic"),
xs:dateTime("2002-10-10T23:02:12Z"),
xs:date("2002-10-10Z"),
xs:time("23:02:12Z"),
xs:duration("P12M"),

(: Sub-types of xs:decimal :)
xs:dayTimeDuration("PT1S"),
xs:yearMonthDuration("P1M"),

xs:float("3e3"),
xs:double("4e4"),
xs:decimal("2.0"),
(: Sub-types of xs:decimal :)
xs:integer("16"),
xs:nonPositiveInteger("0"),
xs:negativeInteger("-4"),
xs:long("5"),
xs:int("6"),
xs:short("7"),
xs:byte("8"),
xs:nonNegativeInteger("9"),
xs:unsignedLong("10"),
xs:unsignedInt("11"),
xs:unsignedShort("12"),
xs:unsignedByte("13"),
xs:positiveInteger("14"),

xs:gYearMonth("1976-02Z"),
xs:gYear("2005-12:00"),
xs:gMonthDay("--12-25-14:00"),
xs:gDay("---25-14:00"),
xs:gMonth("--12-14:00"),
xs:boolean("true"),
xs:base64Binary("aaaa"),
xs:hexBinary("FFFF"),
xs:anyURI("http://example.com/"),
xs:QName("localName"),

xs:string("An xs:string"),
(: Sub-types of xs:string :)
xs:normalizedString("normalizedString"),
xs:token("token"),
xs:language("language"),
xs:NMTOKEN("NMTOKEN"),
xs:Name("Name"),
xs:NCName("NCName"),
xs:ID("ID"),
xs:IDREF("IDREF"),
xs:ENTITY("ENTITY")