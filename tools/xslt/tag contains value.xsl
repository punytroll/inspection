<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="xml" omit-xml-declaration="yes" indent="yes"/>
	
	<xsl:template match="@* | node()">
		<xsl:copy>
			<xsl:apply-templates select="@* | node()"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="tag">
		<tag>
			<xsl:choose>
				<xsl:when test="@name">
					<xsl:attribute name="name">
						<xsl:value-of select="@name"/>
					</xsl:attribute>
					<xsl:if test="*">
						<xsl:apply-templates select="text()[1]"/>
						<value>
							<xsl:apply-templates select="text()[1]"/>
							<xsl:text disable-output-escaping="yes">&#x9;</xsl:text>
							<xsl:apply-templates select="*"/>
							<xsl:apply-templates select="text()[1]"/>
						</value>
						<xsl:apply-templates select="text()[2]"/>
					</xsl:if>
				</xsl:when>
				<xsl:otherwise>
					<xsl:apply-templates/>
				</xsl:otherwise>
			</xsl:choose>
		</tag>
	</xsl:template>
</xsl:stylesheet>
